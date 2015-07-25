/*****************************************************************************
 * Raspberry Pi GPIO counter example with simple integrated webserver
 *****************************************************************************/
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <wiringPi.h>


/*****************************************************************************
 * Global defines
 *****************************************************************************/
#define INPUT_PIN           7
#define WEBSERVER_PORT      3333


/*****************************************************************************
 * Global variables
 *****************************************************************************/
static int raspi_isr_counter = 0;               /**< input counter */
static char raspi_html_buf[1024];               /**< html buffer */


/*****************************************************************************/
/** GPIO counter function
 */
static void raspi_counter(
    void
)
{
    raspi_isr_counter++;
}


/*****************************************************************************/
/** Main function with webserver
 */
int main(void)
{
    int res;                                    /**< result */
    int fd;                                     /**< socket descriptor */
    int new_fd;                                 /**< client socket desc */
    int one = 1;                                /**< socket option */
    socklen_t cli_size;                         /**< client addr size */
    int raspi_gpio_status = 0;                  /**< GPIO status */
    struct sockaddr_in srv_addr;                /**< webserver addr */
    struct sockaddr_in cli_addr;                /**< client addr */

    /* setup wiringPi */
    res = wiringPiSetup();
    if (0 > res) {
        fprintf(stderr, "Error: setup wiringPi failed: %s\n", strerror(errno));
        return 1;
    }

    /* setup GPIO counter function */
    res = wiringPiISR(INPUT_PIN, INT_EDGE_FALLING, &raspi_counter);
    if (0 > res) {
        fprintf(stderr, "Error: setup ISR pin failed: %s\n", strerror(errno));
        return 1;
    }

    /* open socket for webserver */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > fd) {
        fprintf(stderr, "Error: opening socket failed: %s\n", strerror(errno));
        return 1;
    }

    /* make socket reusable */
    res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    if (0 > res) {
        fprintf(stderr, "Error: setting SO_REUSEADDR on socket failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    /* init socket struct */
    memset(&srv_addr, 0, sizeof(struct sockaddr_in));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(WEBSERVER_PORT);

    /* bind socket to address */
    res = bind(fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (0 > res) {
        fprintf(stderr, "Error: binding socket to port %u failed: %s\n", WEBSERVER_PORT, strerror(errno));
        close(fd);
        return 1;
    }

    /* start main loop */
    while (1) {

        /* listen to incoming requests */
        listen(fd, 5);
        cli_size = sizeof(struct sockaddr_in);

        /* accept request */
        new_fd = accept(fd, (struct sockaddr *) &cli_addr, &cli_size);
        if (0 > new_fd) {
            fprintf(stderr, "Error: accepting socket failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }

        raspi_gpio_status = digitalRead(INPUT_PIN);

        /* ignore request and only print counter value */
        snprintf(raspi_html_buf, sizeof(raspi_html_buf), "<html><body>Z&auml;hler: %u<br/>GPIO Status: %u</body></html>", raspi_isr_counter, raspi_gpio_status);
        res = write(new_fd, raspi_html_buf, strlen(raspi_html_buf));
        if (0 > res) {
            fprintf(stderr, "Error: writing to socket failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }

        /* close client descriptor */
        close(new_fd);
    }

    return 0;
}
